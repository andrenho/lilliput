import React, { Component } from 'react';

class HexViewer extends Component {

    upstreamData() {
        return this.props.binary.map(v => v).join(', ');
    }

    render() {
        return <textarea readOnly value={ this.upstreamData() } />;
    }
}

export default HexViewer;